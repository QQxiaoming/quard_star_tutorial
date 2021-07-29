(:*******************************************************:)
(: Test: K2-BaseURIFunc-11                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure comments pick up the correct xml:base value. :)
(:*******************************************************:)
let $i := <e xml:base="http://www.example.com/"><!-- content --></e>
                return base-uri($i/comment()[1])