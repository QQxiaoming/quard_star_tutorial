(:*******************************************************:)
(: Test: K2-BaseURIFunc-28                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Check the document URI and base URI simultaneously for a computed document node. :)
(:*******************************************************:)
declare base-uri "http://example.com/";
let $i := document {()}
return ("Base URI:", base-uri($i), "Document URI:", document-uri($i))