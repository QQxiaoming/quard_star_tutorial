(:*******************************************************:)
(: Test: K-StringJoinFunc-5                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `string-join(('Blow, ', 'blow, ', 'thou ', 'winter ', 'wind!'), '') eq "Blow, blow, thou winter wind!"`. :)
(:*******************************************************:)
string-join(('Blow, ', 'blow, ', 'thou ', 'winter ', 'wind!'), '')
			eq "Blow, blow, thou winter wind!"