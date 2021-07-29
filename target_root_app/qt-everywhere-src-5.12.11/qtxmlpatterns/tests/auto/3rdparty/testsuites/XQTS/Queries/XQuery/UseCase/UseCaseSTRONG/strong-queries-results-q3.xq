module namespace p="http://www.example.com/xq/postals";
import schema namespace ipo = "http://www.example.com/IPO";
import schema namespace pst = "http://www.example.com/postals";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

declare function p:postal-ok($a as element(*, ipo:UKAddress))
  as xs:boolean
{
  some $i in $input-context/pst:postals/element(pst:row)
  satisfies $i/pst:city = $a/city
       and starts-with($a/postcode, $i/pst:prefix)
}; 