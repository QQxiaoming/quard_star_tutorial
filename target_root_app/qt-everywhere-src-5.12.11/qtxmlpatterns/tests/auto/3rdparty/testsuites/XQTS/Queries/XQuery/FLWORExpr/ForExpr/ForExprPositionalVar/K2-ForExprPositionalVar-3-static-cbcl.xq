(:*******************************************************:)
(: Test: K2-ForExprPositionalVar-3                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-10-03T14:53:32+01:00                       :)
(: Purpose: Usage of two positional variable references. :)
(:*******************************************************:)
declare construction strip;
let $tree := <e>
    <a id="1"/>
    <a id="2"/>
    <a id="3"/>
</e>
for $i at $pos in ("a", "b", "c")
return ($tree/a/@id = $pos, $pos)
