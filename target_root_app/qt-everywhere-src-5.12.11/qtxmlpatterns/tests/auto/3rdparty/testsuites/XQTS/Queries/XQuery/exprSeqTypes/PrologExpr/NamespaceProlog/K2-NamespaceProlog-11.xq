(:*******************************************************:)
(: Test: K2-NamespaceProlog-11                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Undeclare the 'fn' prefix.                   :)
(:*******************************************************:)

declare namespace fn = "";
fn:untypedAtomic("string")
          