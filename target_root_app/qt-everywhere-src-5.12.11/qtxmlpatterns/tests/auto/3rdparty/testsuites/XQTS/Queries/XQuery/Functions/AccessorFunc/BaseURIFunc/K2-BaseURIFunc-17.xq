(:*******************************************************:)
(: Test: K2-BaseURIFunc-17                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure computed attributes don't pick up the base-uri from the static context. :)
(:*******************************************************:)
declare base-uri "http://www.example.com";
                empty(base-uri(attribute name {"data"}))