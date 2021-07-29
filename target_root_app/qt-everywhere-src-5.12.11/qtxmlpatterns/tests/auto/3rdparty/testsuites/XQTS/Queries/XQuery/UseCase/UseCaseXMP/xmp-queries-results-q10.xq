(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<results>
  {
    let $doc := $input-context
    for $t in distinct-values($doc//book/title)
    let $p := $doc//book[title = $t]/price
    return
      <minprice title="{ $t }">
        <price>{ min($p) }</price>
      </minprice>
  }
</results> 