(:*******************************************************:)
(: Test: K2-BaseURIFunc-23                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that the base URI is empty for computed PI constructors. :)
(:*******************************************************:)
declare base-uri "http://example.com/baseURI";
empty(base-uri(processing-instruction target {"data"}))