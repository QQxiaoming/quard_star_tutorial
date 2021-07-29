(:*******************************************************:)
(: Test: function-declaration-009.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Thu Jun  2 00:24:56 2005                        :)
(: Purpose: Demonstrate function declaration in different combination :)
(:********************************************************************:)

declare function local:fact($n as xs:integer) as xs:integer
{
    if ($n < 2)
        then 1
        else $n * local:fact(($n)-1)
};
<table>
{
   for $i in 1 to 10
       return
             <tr>
                <td>{$i}! = {local:fact($i)}</td>
             </tr>
}
</table>
