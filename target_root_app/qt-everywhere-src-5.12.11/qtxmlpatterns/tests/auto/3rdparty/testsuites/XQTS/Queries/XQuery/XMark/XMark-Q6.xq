(: Written By: Frans Englich(maintainer, not original author) :)
(: Purpose: How many items are listed on all continents? :)
(: Date: 2007-03-09 :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<XMark-result-Q6>
  {
    let $auction := $input-context return
    for $b in $auction//site/regions return count($b//item)
  }
</XMark-result-Q6>
