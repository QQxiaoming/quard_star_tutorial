import schema namespace ipo="http://www.example.com/IPO";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

declare function local:comments-for-element( $e as element() )
  as element(ipo:comment)*
{
  $e/element(ipo:comment)
};

for $p in $input-context//element(ipo:purchaseOrder)
where $p/shipTo/name="Helen Zoe"
  and $p/@orderDate = xs:date("1999-12-01")
return 
  <comments name="{$p/shipTo/name}" date="{$p/@orderDate}">
   {
     local:comments-for-element( $p )
   }
   {
     for $i in $p//item
     return local:comments-for-element( $i )
   }
  </comments> 