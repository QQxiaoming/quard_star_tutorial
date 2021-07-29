(: Written By: Frans Englich(maintainer, not original author) :)
(: Purpose: How many pieces of prose are in our database? :)
(: Date: 2007-03-09 :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<XMark-result-Q7>
  {
    let $auction := $input-context return
    for $p in $auction/site
    return count($p//description) + count($p//annotation) + count($p//emailaddress)
  }
</XMark-result-Q7>
