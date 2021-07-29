declare namespace xlink = "http://www.w3.org/1999/xlink";
(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<Q4 xmlns:xlink="http://www.w3.org/1999/xlink">
  {
    for $hr in $input-context//@xlink:href
    return <ns>{ $hr }</ns>
  }
</Q4>