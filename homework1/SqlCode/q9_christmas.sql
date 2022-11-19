WITH Base AS (
	SELECT
		Product.Id,
		ProductName AS Name 
	FROM
		Product,
		OrderDetail,
		'Order',
		Customer 
	WHERE
		Product.Id = ProductId 
		AND 'Order'.Id = OrderId 
		AND CustomerId = Customer.Id 
		AND CompanyName = 'Queen Cozinha' 
		AND OrderDate LIKE '2014-12-25%' 
	GROUP BY
		ProductId 
	),
	TEMP AS ( SELECT row_number( ) OVER ( ORDER BY Id ) AS Seqnum, Name FROM Base ),
	Res AS (
	SELECT
		Seqnum,
		Name 
	FROM
	TEMP 
	WHERE
		Seqnum = 1 UNION ALL
	SELECT TEMP
		.Seqnum,
		Res.name || ', ' || TEMP.name 
	FROM
		TEMP,
		Res 
	WHERE
		TEMP.Seqnum = Res.Seqnum + 1 
	) SELECT
	Name 
FROM
	Res 
ORDER BY
	Seqnum DESC 
	LIMIT 1;