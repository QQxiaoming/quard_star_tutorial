(:*******************************************************:)
(: Test: K2-ComputeConAttr-43                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: It's not allowed to bind the xmlns namespace to any prefix(#2). :)
(:*******************************************************:)
declare namespace prefix = "http://www.w3.org/2000/xmlns/";
    <e>{attribute {"prefix:localName"} {"content"}}</e>