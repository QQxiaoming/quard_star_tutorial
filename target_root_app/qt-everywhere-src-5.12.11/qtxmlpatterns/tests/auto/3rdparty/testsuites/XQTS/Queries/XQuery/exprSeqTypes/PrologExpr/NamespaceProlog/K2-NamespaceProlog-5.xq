(:*******************************************************:)
(: Test: K2-NamespaceProlog-5                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: It is ok to undeclare a non-bound namespace. :)
(:*******************************************************:)

declare namespace thisPrefixIsNotBoundExampleCom = "";
true()
          