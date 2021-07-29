import schema namespace ipo="http://www.example.com/IPO";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

declare function local:deadbeat( $b as element(billTo, ipo:USAddress) )
  as xs:boolean
{
   $b/name = doc("http://www.usa-deadbeats.com/current")/deadbeats/row/name
}; 

for $p in $input-context//element(ipo:purchaseOrder)
where $p/@orderDate = date("2004-12-01")
  and local:deadbeat( $p/element(billTo) )
return <warning>{ $p/billTo/name, "is a known deadbeat!" }</warning>
