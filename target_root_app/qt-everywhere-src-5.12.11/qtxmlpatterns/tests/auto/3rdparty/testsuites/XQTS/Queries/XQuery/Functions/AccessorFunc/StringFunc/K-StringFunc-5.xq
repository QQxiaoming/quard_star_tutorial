(:*******************************************************:)
(: Test: K-StringFunc-5                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Apply fn:string() on xs:string.              :)
(:*******************************************************:)
string-length(string(xs:string(current-time()))) gt 2