import schema namespace ipo="http://www.example.com/IPO";
import schema namespace pst="http://www.example.com/postals";
import schema namespace zips="http://www.example.com/zips";

import module namespace zok="http://www.example.com/xq/zips";
import module namespace pok="http://www.example.com/xq/postals";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

declare function local:address-ok($a as element(*, ipo:Address))
 as xs:boolean
{
  typeswitch ($a)
      case $zip as element(*, ipo:USAddress)
           return zok:zip-ok($zip)
      case $postal as element(*, ipo:UKAddress )
           return pok:postal-ok($postal) 
      default return false()
};

for $p in $input-context//element(ipo:purchaseOrder)
where not( local:address-ok($p/shipTo) and local:address-ok($p/billTo))
return $p 