(: Written By: Frans Englich(maintainer, not original author) :)
(: Purpose: Return the IDs of those auctions that have one
            or more keywords in emphasis. (cf. Q15) :)
(: Date: 2007-03-09 :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<XMark-result-Q16>
  {
    let $auction := $input-context return
    for $a in $auction/site/closed_auctions/closed_auction
    where
      not(
        empty(
          $a/annotation/description/parlist/listitem/parlist/listitem/text/emph/
           keyword/
           text()
        )
      )
    return <person id="{$a/seller/@person}"/>
  }
</XMark-result-Q16>
