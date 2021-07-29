(:*******************************************************:)
(: Test: K2-SeqUnion-4                                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure node deduplication is done on the operands. :)
(:*******************************************************:)
let $i := <e>
<a/>
    <b/>
        <c/>
</e>/a
, $t := $i/following-sibling::b
return (($i union ($i, $i)),
(($t, $t) union $t))