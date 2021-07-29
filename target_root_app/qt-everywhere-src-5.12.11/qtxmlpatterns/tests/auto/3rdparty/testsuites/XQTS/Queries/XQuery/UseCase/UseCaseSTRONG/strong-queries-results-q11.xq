import schema namespace ipo="http://www.example.com/IPO";
import module namespace calc = "http://www.example.com/calc";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $p in $input-context//element(ipo:purchaseOrder)
where $p/shipTo/name="Helen Zoe"
   and $p/@orderDate = xs:date("1999-12-01")
return calc:total-price($p//ipo:item) 