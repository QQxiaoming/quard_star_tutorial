(:*******************************************************:)
(: Test: K2-SeqIntersect-41                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure node deduplication is done.           :)
(:*******************************************************:)
<r>
    {
    let $i := <e>
                <a/>
                <b/>
                <c/>
              </e>/a
    , $t := $i/following-sibling::b
    return (($i intersect ($i, $i)),
            (($t, $t) intersect $t))
    }
</r>