(:*******************************************************:)
(: Test: K2-NodeTest-20                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use document-node() with an unkown schema-element() type. :)
(:*******************************************************:)
declare namespace ex = "http://www.example.com/";
              document-node(schema-element(ex:thisTypeIsNotRecognizedExample.Com))