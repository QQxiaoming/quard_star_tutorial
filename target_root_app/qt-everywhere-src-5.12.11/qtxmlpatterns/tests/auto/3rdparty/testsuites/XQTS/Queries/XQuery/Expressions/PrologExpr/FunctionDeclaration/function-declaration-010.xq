(:*******************************************************:)
(: Test: function-declaration-010.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Thu Jun  2 00:24:56 2005                        :)
(: Purpose: Demonstrate function declaration in different combination :)
(:********************************************************************:)

declare function local:prnt($n as xs:integer,$n2 as xs:string, $n3 as xs:date,
                          $n4 as xs:long, $n5 as xs:string, $n6 as xs:decimal)
{
    if ($n < 2)
        then 1
        else concat($n, " ",$n2," ",$n3," ",$n4," ",$n5," ",$n6)
};
<table>
{
     <td>Value is =  {local:prnt(4,xs:string("hello"),xs:date("2005-02-22"),
                      xs:long(5),xs:string("well"),xs:decimal(1.2))}</td>
}
</table>
