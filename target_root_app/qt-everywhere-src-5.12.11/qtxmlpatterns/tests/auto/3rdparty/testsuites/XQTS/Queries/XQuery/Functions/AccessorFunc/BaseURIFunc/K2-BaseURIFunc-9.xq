(:*******************************************************:)
(: Test: K2-BaseURIFunc-9                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure processing-instructions pick up the correct xml:base value. :)
(:*******************************************************:)
let $i := <e xml:base="http://www.example.com/"><?target data?></e>
                return base-uri($i/processing-instruction()[1])