(: Written By: Frans Englich(maintainer, not original author) :)
(: Purpose:  Return the initial increases of all open auctions. :)
(: Date: 2007-03-09 :)
(: Date: 2007-03-09 :)
(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<XMark-result-Q2>
  {
    let $auction := $input-context return
    for $b in $auction/site/open_auctions/open_auction
    return <increase>{$b/bidder[1]/increase/text()}</increase>
  }
</XMark-result-Q2>
