(:*******************************************************:)
(: Test: K2-SeqExprCast-1                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Cast to xs:QName where the prefix is declared in the prolog. :)
(:*******************************************************:)
declare namespace myPrefix = "http://example.com/";
"myPrefix:ncname" cast as xs:QName eq QName("http://example.com/", "anotherPrefix:ncname")