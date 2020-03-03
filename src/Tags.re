[@react.component]
let make = (~tags) => {
  Js.log(tags);
  <div> "tags"->ReasonReact.string </div>;
};
