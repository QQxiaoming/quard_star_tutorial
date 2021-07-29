(:*******************************************************:)
(: Test: K-IRIToURIfunc-4                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Invoke fn:normalize-space() on the return value of fn:iri-to-uri(). Implementations supporting the static typing feature may raise XPTY0004. :)
(:*******************************************************:)
normalize-space(iri-to-uri(("somestring", current-time())[1])) eq "somestring"