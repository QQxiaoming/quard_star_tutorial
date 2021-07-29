(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<section_list>
  {
    for $s in $input-context//section
    let $f := $s/figure
    return
        <section title="{ $s/title/text() }" figcount="{ count($f) }"/>
  }
</section_list> 