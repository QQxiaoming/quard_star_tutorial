(: Written By: Frans Englich(maintainer, not original author) :)
(: Purpose: Give an alphabetically ordered list of all
            items along with their location. :)
(: Date: 2007-03-09 :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<XMark-result-Q19>
  {
    let $auction := $input-context return
    for $b in $auction/site/regions//item
    let $k := $b/name/text()
    stable order by zero-or-one($b/location) ascending empty greatest
    return <item name="{$k}">{$b/location/text()}</item>
  }
</XMark-result-Q19>
