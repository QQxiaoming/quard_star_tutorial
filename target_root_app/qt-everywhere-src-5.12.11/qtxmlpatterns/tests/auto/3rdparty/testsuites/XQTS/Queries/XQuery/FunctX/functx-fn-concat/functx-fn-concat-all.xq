(:**************************************************************:)
(: Test: functx-fn-concat-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(concat('a', 'b'), concat('a', 'b', 'c'), concat('a', (), 'b', '', 'c'), concat('a', <x>b</x>, 'c'))