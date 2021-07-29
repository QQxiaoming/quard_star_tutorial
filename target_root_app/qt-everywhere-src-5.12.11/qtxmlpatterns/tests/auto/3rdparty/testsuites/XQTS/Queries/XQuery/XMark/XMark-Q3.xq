(: Written By: Frans Englich(maintainer, not original author) :)
(: Purpose: Return the IDs of all open auctions whose current
            increase is at least twice as high as the initial increase. :)
(: Date: 2007-03-09 :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<XMark-result-Q3>
  {
    let $auction := $input-context return
    for $b in $auction/site/open_auctions/open_auction
    where zero-or-one($b/bidder[1]/increase/text()) * 2.0 <= $b/bidder[last()]/increase/text()
    return <increase first="{$b/bidder[1]/increase/text()}"
                     last="{$b/bidder[last()]/increase/text()}"/>
  }
</XMark-result-Q3>
