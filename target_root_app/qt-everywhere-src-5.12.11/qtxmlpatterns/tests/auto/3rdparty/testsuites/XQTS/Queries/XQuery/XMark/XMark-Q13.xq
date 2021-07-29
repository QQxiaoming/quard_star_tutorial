(: Written By: Frans Englich(maintainer, not original author) :)
(: Purpose: List the names of items registered in Australia along with
            their descriptions. :)
(: Date: 2007-03-09 :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<XMark-result-Q13>
  {
    let $auction := $input-context return
    for $i in $auction/site/regions/australia/item
    return <item name="{$i/name/text()}">{$i/description}</item>
  }
</XMark-result-Q13>
