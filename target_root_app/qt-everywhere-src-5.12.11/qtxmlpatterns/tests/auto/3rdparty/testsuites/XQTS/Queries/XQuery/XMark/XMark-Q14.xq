(: Written By: Frans Englich(maintainer, not original author) :)
(: Purpose: Return the names of all items whose description contains the
            word `gold'. :)
(: Date: 2007-03-09 :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<XMark-result-Q14>
  {
    let $auction := $input-context return
    for $i in $auction/site//item
    where contains(string(exactly-one($i/description)), "gold")
    return $i/name/text()
  }
</XMark-result-Q14>
