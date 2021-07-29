(:*******************************************************:)
(: Test: K2-NameTest-39                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Specifying an unknown type in schema-element() is an error(with namespace). :)
(:*******************************************************:)
declare namespace e = "http://www.example.com/";
                  schema-element(e:thisTypeDoesNotExistExample.Com)