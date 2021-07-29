(:*******************************************************:)
(: Test: K2-BaseURIFunc-27                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that the base URI is set for computed documents. :)
(:*******************************************************:)
declare base-uri "http://example.com/BASEURI";
base-uri(document {()})