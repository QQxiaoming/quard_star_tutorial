(:*******************************************************:)
(: Test: default_namespace-008.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Aug  9 03:59:40 2005                        :)
(: Purpose: Demonstrates Default namespace declaration,  :)
(:          facilitates the use of unprefixed QNames     :)
(:*******************************************************:)

declare default function namespace "http://example.org";
declare function fact($n as xs:integer) as xs:integer
{
    if ($n < 2)
        then 1
        else $n * fact($n - 1)
};
declare variable $ten := fact(10);
<table>
{
   for $i in 1 to 10
      return
            <tr>
               <td>10!/{$i}! = {$ten div fact($i)}</td>
            </tr>
}
</table>
