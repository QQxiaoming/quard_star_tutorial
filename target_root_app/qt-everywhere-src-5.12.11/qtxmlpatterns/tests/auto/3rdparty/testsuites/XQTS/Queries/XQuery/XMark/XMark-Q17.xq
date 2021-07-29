(: Written By: Frans Englich(maintainer, not original author) :)
(: Purpose: Which persons don't have a homepage? :)
(: Date: 2007-03-09 :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<XMark-result-Q17>
  {
    let $auction := $input-context return
    for $p in $auction/site/people/person
    where empty($p/homepage/text())
    return <person name="{$p/name/text()}"/>
  }
</XMark-result-Q17>
