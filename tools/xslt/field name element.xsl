<?xml version="1.0"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="xml" omit-xml-declaration="yes" indent="yes"/>
	
	<xsl:template match="@* | node()">
		<xsl:copy>
			<xsl:apply-templates select="@* | node()"/>
		</xsl:copy>
	</xsl:template>
	
	<xsl:template match="field[@name]">
		<xsl:copy>
			<xsl:apply-templates select="@*[name() != 'name']"/>
			<xsl:apply-templates select="text()[1]"/>
			<name>
				<xsl:apply-templates select="text()[1]"/>
				<xsl:text disable-output-escaping="yes">&#x9;</xsl:text>
				<string>
					<xsl:value-of select="@name"/>
				</string>
				<xsl:apply-templates select="text()[1]"/>
			</name>
			<xsl:apply-templates select="node()"/>
		</xsl:copy>
	</xsl:template>
</xsl:stylesheet>
