import schema namespace ipo="http://www.example.com/IPO";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)                    

declare function local:names-match( $s as element(shipTo), 
                                    $b as element(billTo) )
  as xs:boolean
{
     $s/name = $b/name
};
 
for $p in doc("ipo.xml")//element(ipo:purchaseOrder)
where not( local:names-match( $p/shipTo, $p/billTo ) )
return $p 