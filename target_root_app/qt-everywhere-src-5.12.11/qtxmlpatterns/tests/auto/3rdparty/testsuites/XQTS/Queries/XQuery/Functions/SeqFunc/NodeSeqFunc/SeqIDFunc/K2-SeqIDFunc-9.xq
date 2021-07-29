(:*******************************************************:)
(: Test: K2-SeqIDFunc-9                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use an invalid target node.                  :)
(:*******************************************************:)
let $i := document {<e>
                <e/>
                <e/>
                <e/>
                <e/>
                <e/>
                <e/>
                <e/>
                <b xml:id="foo"/>
                <e/>
            </e>}
        return id("foo", $i)
        