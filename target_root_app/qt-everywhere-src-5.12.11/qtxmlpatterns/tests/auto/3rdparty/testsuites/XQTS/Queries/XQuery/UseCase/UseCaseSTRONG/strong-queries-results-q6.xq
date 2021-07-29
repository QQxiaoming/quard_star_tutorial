import schema namespace ipo="http://www.example.com/IPO";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $p in $input-context//element(ipo:purchaseOrder),
    $b in $p/billTo
where not( $b instance of element(*, ipo:USAddress))
  and exists( $p//USPrice )
return $p 