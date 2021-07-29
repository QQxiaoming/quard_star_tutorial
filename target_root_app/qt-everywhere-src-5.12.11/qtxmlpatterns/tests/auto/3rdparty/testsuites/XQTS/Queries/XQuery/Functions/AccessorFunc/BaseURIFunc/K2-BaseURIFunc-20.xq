(:*******************************************************:)
(: Test: K2-BaseURIFunc-20                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure computed attributes pick up the resolved base-uri of the parent(#2). :)
(:*******************************************************:)
declare base-uri "http://www.example.com/";
                let $i := <e xml:base = "foo/../xml" attr="foo">
                </e>
                return base-uri($i/@xml:base)