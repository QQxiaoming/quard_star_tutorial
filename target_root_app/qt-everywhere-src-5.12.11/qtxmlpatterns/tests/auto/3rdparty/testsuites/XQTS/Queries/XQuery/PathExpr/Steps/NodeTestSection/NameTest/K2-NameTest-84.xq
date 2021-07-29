(:*******************************************************:)
(: Test: K2-NameTest-84                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Axis names are reserved function names.      :)
(:*******************************************************:)
declare default function namespace "http://www.example.com/";
declare namespace e =  "http://www.example.com/";
declare function element()
{
    1
};
e:element()
