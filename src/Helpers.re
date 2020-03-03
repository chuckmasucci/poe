let reduceAndSort = tags => {
  tags
  |> Js.Array.filteri((item, index) =>
       Js.Array.indexOf(item, tags) === index
     )
  |> Js.Array.sortInPlace;
};
