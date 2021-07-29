(:*******************************************************:)
(: Test: K2-SeqUnion-1                                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure nodes are deduplicated and sorted even though one of the operands is the empty sequence. :)
(:*******************************************************:)
<r>
    {
        let $i := <e>
                    <a/>
                    <b/>
                  </e>
        return ($i/b, $i/a, $i/b, $i/a) | ()
    }
    </r>