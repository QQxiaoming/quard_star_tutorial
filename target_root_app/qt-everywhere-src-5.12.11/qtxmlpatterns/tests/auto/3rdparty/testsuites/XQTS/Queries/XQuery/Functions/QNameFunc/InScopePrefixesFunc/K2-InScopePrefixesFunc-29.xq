(:*******************************************************:)
(: Test: K2-InScopePrefixesFunc-29                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use a computed element constructor which undeclares the default namespace, as operand to a path expression. :)
(:*******************************************************:)
declare default element namespace "http://www.example.com/";
<a2/>/element e
      {
          element {QName("", "a")} {}
      }