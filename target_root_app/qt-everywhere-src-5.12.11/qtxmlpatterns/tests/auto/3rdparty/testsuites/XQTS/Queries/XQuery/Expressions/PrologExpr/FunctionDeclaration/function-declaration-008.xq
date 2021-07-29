(:*******************************************************:)
(: Test: function-declaration-008.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Thu Jun  2 00:24:56 2005                        :)
(: Purpose: Demonstrate function declaration in different combination :)
(:********************************************************************:)

declare namespace my = "urn:foo";
declare function my:fact($n  as xs:integer) as xs:integer
{
    if ($n < 2)
        then 1
        else $n * my:fact($n - 1)
};
declare variable $my:ten := my:fact(10);
<table>
{
   for $i in 1 to 10
      return
            <tr>
               <td>10!/{$i}! = {$my:ten div my:fact($i)}</td>
            </tr>
}
</table>
