(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $p in $input-context//element(ipo:purchaseOrder)
where $p/shipTo/name="Helen Zoe"
  and $p/@orderDate = xs:date("1999-12-01")
return local:comment-text( $p//ipo:customerComment ) 