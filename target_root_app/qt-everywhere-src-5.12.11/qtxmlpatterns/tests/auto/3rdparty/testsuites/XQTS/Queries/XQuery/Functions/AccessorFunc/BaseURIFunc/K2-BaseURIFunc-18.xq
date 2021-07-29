(:*******************************************************:)
(: Test: K2-BaseURIFunc-18                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure computed attributes pick up the base-uri from parent. :)
(:*******************************************************:)
declare base-uri "http://www.example.com";
                let $i := <e attr="foo"></e>
                return base-uri($i/@attr)