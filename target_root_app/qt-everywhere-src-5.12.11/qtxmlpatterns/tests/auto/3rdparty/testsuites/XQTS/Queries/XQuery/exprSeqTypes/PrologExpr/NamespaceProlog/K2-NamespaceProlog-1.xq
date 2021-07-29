(:*******************************************************:)
(: Test: K2-NamespaceProlog-1                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A namespace declaration cannot occur twice for the same prefix, no matter what. :)
(:*******************************************************:)

declare namespace myPrefix = "http://example.com/";
declare namespace myPrefix = "";
1
          