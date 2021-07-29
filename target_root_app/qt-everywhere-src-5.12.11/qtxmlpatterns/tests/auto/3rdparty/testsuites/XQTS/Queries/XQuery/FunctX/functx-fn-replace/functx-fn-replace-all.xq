(:**************************************************************:)
(: Test: functx-fn-replace-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(replace('query', 'r', 'as'), replace('query', 'qu', 'quack'), replace('query', '[ry]', 'l'), replace('query', '[ry]+', 'l'), replace('query', 'z', 'a'), replace('query', 'query', ''), replace( (), 'r', 'as'), replace('Chapter', '(Chap)|(Chapter)', 'x'), replace('reluctant', 'r.*t', 'X'), replace('reluctant', 'r.*?t', 'X'), replace('aaah', 'a{2,3}', 'X'), replace('aaah', 'a{2,3}?', 'X'), replace('aaaah', 'a{2,3}', 'X'), replace('aaaah', 'a{2,3}?', 'X'), replace('Chap 2...Chap 3...Chap 4...',
        'Chap (\d)', 'Sec $1.0'), replace('abc123', '([a-z])', '$1x'), replace('2315551212',
        '(\d{3})(\d{3})(\d{4})', '($1) $2-$3'), replace('2006-10-18',
        '\d{2}(\d{2})-(\d{2})-(\d{2})',
        '$2/$3/$1'), replace('25', '(\d+)', '\$$1.00'))