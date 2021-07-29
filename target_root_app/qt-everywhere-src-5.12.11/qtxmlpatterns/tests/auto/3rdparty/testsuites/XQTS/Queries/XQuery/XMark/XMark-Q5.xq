(: Written By: Frans Englich(maintainer, not original author) :)
(: Purpose: How many sold items cost more than 40? :)
(: Date: 2007-03-09 :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<XMark-result-Q5>
  {
    let $auction := $input-context return
    count(for $i in $auction/site/closed_auctions/closed_auction
          where $i/price/text() >= 40.0
          return $i/price)
  }
</XMark-result-Q5>
