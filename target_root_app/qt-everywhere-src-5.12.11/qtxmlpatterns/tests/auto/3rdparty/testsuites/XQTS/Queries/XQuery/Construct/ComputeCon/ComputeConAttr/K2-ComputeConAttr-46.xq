(:*******************************************************:)
(: Test: K2-ComputeConAttr-46                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The default element namespace declaration doesn't affect attribute declarations. :)
(:*******************************************************:)
declare default element namespace "http://www.example.com/";
    <e>{attribute xmlns {"content"}}</e>