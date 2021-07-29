(: Written By: Frans Englich(maintainer, not original author) :)
(: Purpose: Print the keywords in emphasis in annotations of closed auctions. :)
(: Date: 2007-03-09 :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<XMark-result-Q15>
  {
    let $auction := $input-context return
    for $a in
      $auction/site/closed_auctions/closed_auction/annotation/description/parlist/
       listitem/
       parlist/
       listitem/
       text/
       emph/
       keyword/
       text()
    return <text>{$a}</text>
  }
</XMark-result-Q15>
