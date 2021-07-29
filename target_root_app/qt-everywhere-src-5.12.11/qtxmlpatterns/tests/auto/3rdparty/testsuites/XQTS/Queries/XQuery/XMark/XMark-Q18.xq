(: Written By: Frans Englich(maintainer, not original author) :)
(: Purpose: Convert the currency of the reserve of all
            open auctions to another currency. :)
(: Date: 2007-03-09 :)

declare namespace local = "http://www.example.com/";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

declare function local:convert($v as xs:decimal?) as xs:decimal?
{
  2.20371 * $v (: convert Dfl to Euro :)
};
<XMark-result-Q18>
  {
    let $auction := $input-context return
    for $i in $auction/site/open_auctions/open_auction
    return local:convert(zero-or-one($i/reserve))
  }
</XMark-result-Q18>
