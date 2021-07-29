(: Written By: Frans Englich(maintainer, not original author) :)
(: Purpose: List the names of persons and the number of items they bought.
            (joins person, closed\_auction). :)
(: Date: 2007-03-09 :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<XMark-result-Q8>
  {
    let $auction := $input-context return
    for $p in $auction/site/people/person
    let $a := for $t in $auction/site/closed_auctions/closed_auction
              where $t/buyer/@person = $p/@id
              return $t
    return <item person="{$p/name/text()}">{count($a)}</item>
  }
</XMark-result-Q8>
