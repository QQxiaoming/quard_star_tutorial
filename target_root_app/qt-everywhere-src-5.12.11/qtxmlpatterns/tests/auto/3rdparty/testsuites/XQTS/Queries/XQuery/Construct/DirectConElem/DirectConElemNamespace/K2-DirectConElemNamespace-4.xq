(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-4                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A namespace declaration binding to a a one-letter NCName. :)
(:*******************************************************:)
declare namespace p = "http://example.com/QuiteWeirdNamespace";
empty(p:e[1])