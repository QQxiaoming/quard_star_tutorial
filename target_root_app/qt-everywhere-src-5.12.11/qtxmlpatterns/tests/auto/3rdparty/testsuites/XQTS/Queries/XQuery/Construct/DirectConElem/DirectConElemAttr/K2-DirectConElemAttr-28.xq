(:*******************************************************:)
(: Test: K2-DirectConElemAttr-28                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Comments cannot appear inside direct element/attribute constructors. :)
(:*******************************************************:)
<foo attr="value" (:comment:) attr2="value" />