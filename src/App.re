[@bs.val] external fetch: string => Js.Promise.t('a) = "fetch";

type gem('a) = {
  name: string,
  gem_tags: 'a,
};
type title('a) = {title: gem('a)};
type gems('a) = array(title('a));
type query = {cargoquery: gems(string)};
type state =
  | LoadingGems
  | ErrorLoadingGems
  | LoadedGems(array(gem(array(string))));

let createGemArray = (json: Js.Json.t) =>
  Js.Json.stringify(json)
  |> Js.String.replaceByRe([%re "/gem tags/gi"], "gem_tags")
  |> Js.Json.parseExn;

let decodeGem = json =>
  Json.Decode.{
    name: json |> field("name", string),
    gem_tags: json |> field("gem_tags", string),
  };

let decodeGemTitle = json =>
  Json.Decode.{title: json |> field("title", decodeGem)};

let decodeQuery = json =>
  Json.Decode.{
    cargoquery: json |> field("cargoquery", array(decodeGemTitle)),
  };

[@react.component]
let make = () => {
  let (state, setState) = React.useState(() => LoadingGems);
  React.useEffect0(() => {
    Js.Promise.(
      fetch(
        "https://pathofexile.gamepedia.com/api.php?"
        ++ "action=cargoquery"
        ++ "&tables=items,skill,skill_gems"
        ++ "&fields=items.name,skill_gems.gem_tags,"
        ++ "&where=items.frame_type%3D%22gem%22"
        ++ "&join_on=items.name%3Dskill_gems._pageName,items.name%3Dskill._pageName"
        ++ "&limit=10"
        ++ "&format=json"
        ++ "&origin=*",
      )
      |> then_(response => response##json())
      |> then_(json => {
           let gems = createGemArray(json) |> decodeQuery;
           let gemsWithArrayTags =
             gems.cargoquery
             ->Belt.Array.map(gem =>
                 {
                   name: gem.title.name,
                   gem_tags: gem.title.gem_tags |> Js.String.split(","),
                 }
               );
           setState(_previousState => LoadedGems(gemsWithArrayTags))
           |> resolve;
         })
      |> catch(_err => {
           Js.log2("error:", _err);
           setState(_previousState => ErrorLoadingGems) |> resolve;
         })
      |> ignore
    );
    None;
  });

  switch (state) {
  | ErrorLoadingGems => React.string("An error occurred!")
  | LoadingGems => React.string("Loading...")
  | LoadedGems(gems) =>
    let gemTagList =
      gems |> Js.Array.map(gem => gem.gem_tags |> Js.Array.joinWith(","));
    let tags = gemTagList |> Js.Array.joinWith(",") |> Js.String.split(",");
    let utags = Helpers.reduceAndSort(tags);

    let filtered =
      gems
      |> Js.Array.filter(gem =>
           gem.gem_tags
           |> Js.Array.joinWith("|")
           |> Js.Re.test_([%re "/(?=.*Fire)|(?=.*Chaos)/gi"])
         );
    <>
      <Tags tags=utags />
      {filtered
       ->Belt.Array.mapWithIndex((i, gem) => {
           <div key={string_of_int(i)}>
             <b> {gem.name}->ReasonReact.string </b>
             <br />
             {gem.gem_tags
              ->Belt.Array.mapWithIndex((i, tag) => {
                  <div key={string_of_int(i)}>
                    {tag}->ReasonReact.string
                  </div>
                })
              ->React.array}
             <br />
           </div>
         })
       ->React.array}
    </>;
  };
};
