(:*******************************************************:)
(: Test: default_namespace-009.xq          :)
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
        else $n * fact(($n)-1)
};
<table>
{
   for $i in 1 to 10
       return
             <tr>
                <td>{$i}! = {fact($i)}</td>
             </tr>
}
</table>
