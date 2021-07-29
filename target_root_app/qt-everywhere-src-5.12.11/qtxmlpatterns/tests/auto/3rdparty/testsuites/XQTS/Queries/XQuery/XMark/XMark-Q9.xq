(: Written By: Frans Englich(maintainer, not original author) :)
(: Purpose: List the names of persons and the names of the items they bought
            in Europe(joins person, closed auction, item). :)
(: Date: 2007-03-09 :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<XMark-result-Q9>
  {
    let $auction := $input-context return
    let $ca := $auction/site/closed_auctions/closed_auction return
    let $ei := $auction/site/regions/europe/item
    for $p in $auction/site/people/person
      let $a := for $t in $ca
                where $p/@id = $t/buyer/@person
                return let $n := for $t2 in $ei where $t/itemref/@item = $t2/@id return $t2
                       return <item>{$n/name/text()}</item>
    return <person name="{$p/name/text()}">{$a}</person>
  }
</XMark-result-Q9>
